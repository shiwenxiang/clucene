#include "TransactionalRAMDirectory.h"

namespace lucene{ namespace store{

  TransactionalRAMDirectory::TransactionalRAMDirectory() {
    RAMDirectory::RAMDirectory();

    // filesToRemoveOnAbort should not free the memory of its values, since
    // files owns the memory.

    // filesToRestoreOnAbort should free the memory both its its values and
    // its keys.
    filesToRestoreOnAbort.setDoDelete(true, DELETE_TYPE_DELETE_ARRAY);
    filesToRestoreOnAbort.setDoDelete(false, DELETE_TYPE_DELETE);

    transOpen = false;
  }

  bool TransactionalRAMDirectory::archiveOrigFileIfNecessary(const char_t* name) {
    // If a file named $name was present when the transaction started and the
    // original RAMFile object has not been archived for restoration upon
    // transaction abort, then do so, and return true.
    // In any other case, return false.
    if (fileExists(name) && filesToRemoveOnAbort.find(name) == filesToRemoveOnAbort.end()) {
      // The file exists, but isn't recorded as having been created after the
      // start of the transaction, so it must've been present at the start of
      // the transaction.

      // Transfer memory ownership of both the key and the value from files to
      // filesToRestoreOnAbort.
      const char_t* origName = files.getKey(name);
      RAMFile* origFile = files.get(name);
      files.remove(name, true, true);
      filesToRestoreOnAbort.put(origName, origFile);

      CND_CONDITION(!fileExists(name), "File should not exist immediately after archival.");
      return true;
    }

    return false;
  }

  void TransactionalRAMDirectory::unarchiveOrigFile(const char_t* name) {
    const char_t* origName = filesToRestoreOnAbort.getKey(name);
    if (origName == NULL) {
      _THROWC("File submitted for unarchival was not archived.");
    }
    RAMFile* origFile = filesToRestoreOnAbort.get(name);
    // Transfer memory ownership back to files from filesToRestoreOnAbort.
    filesToRestoreOnAbort.remove(name, true, true);
    files.put(origName, origFile);
  }

  bool TransactionalRAMDirectory::transIsOpen() {
    return transOpen;
  }

  void TransactionalRAMDirectory::transStart() {
    if (transOpen) {
      _THROWC("Must resolve previous transaction before starting another.");
    }

    CND_CONDITION(filesToRemoveOnAbort.size() == 0,
        "filesToRemoveOnAbort should have been cleared by either its"
        " constructor or transResolved."
      )
    CND_CONDITION(filesToRestoreOnAbort.size() == 0,
        "filesToRestoreOnAbort should have been cleared by either its"
        " constructor or transResolved."
      )

    transOpen = true;
  }

  void TransactionalRAMDirectory::transResolved() {
    // This method implements actions common to both forms of transaction
    // resolution.
    filesToRemoveOnAbort.clear();
    filesToRestoreOnAbort.clear();
    transOpen = false;
  }

  void TransactionalRAMDirectory::transCommit() {
    if (!transOpen) {
      _THROWC("There is no open transaction.");
    }

    // All storage is in memory, so commit is ultra-simple.
    transResolved();
  }

  void TransactionalRAMDirectory::transAbort() {
    if (!transOpen) {
      _THROWC("There is no open transaction.");
    }

    // Delete each file in filesToRemoveOnAbort.
    for (FilenameSet::const_iterator itrDel = filesToRemoveOnAbort.begin();
         itrDel != filesToRemoveOnAbort.end();
         ++itrDel
      )
    {
      const char_t* name = *itrDel;
      const long_t nameLength = stringLength(name);

      // Special exception:
      // Refrain from deleting a lock's flag file, as that would interfere with
      // the operation of the lock.
      if (!(nameLength >= 5 && stringCompare(name + nameLength - 5, _T(".lock")))) {
        RAMDirectory::deleteFile(name);
      }
    }
    // Ownership of the memory of both the key and the value never left files,
    // so there's no need for a special directive to filesToRemoveOnAbort.
    filesToRemoveOnAbort.clear();

    // Now that any new-since-trans-start files with the same names as
    // already-present-at-trans-start files are out of the way, restore each
    // file in filesToRestoreOnAbort.
    lucene::util::VoidList<const char_t*> removeTheseWithoutDeletingMem;
    for (FileMap::const_iterator itr = filesToRestoreOnAbort.begin();
         itr != filesToRestoreOnAbort.end();
         ++itr
      )
    {
      const char_t* name = itr->first;
      files.put(name, itr->second);
      // We've just transferred ownership of the memory of both the key and the
      // value to files; we must now direct filesToRestoreOnAbort not to delete
      // that memory as it removes the entry.  This is performed in a separate
      // loop to avoid modifying filesToRestoreOnAbort while iterating over it.
      removeTheseWithoutDeletingMem.put(name);
    }

    VoidList<const char_t*>::const_iterator itrRem;
    for (itrRem = removeTheseWithoutDeletingMem.begin();
         itrRem != removeTheseWithoutDeletingMem.end();
         ++itrRem
      )
    {
      filesToRestoreOnAbort.remove(*itrRem, true, true);
    }
    CND_CONDITION(filesToRestoreOnAbort.size() == 0, "filesToRestoreOnAbort should be empty.");

    transResolved();
  }

  void TransactionalRAMDirectory::deleteFile(const char_t* name, const bool throwError) {
    if (!transOpen) {
      RAMDirectory::deleteFile(name, throwError);
      return;
    }

    bool wasOriginalAndWasArchived = archiveOrigFileIfNecessary(name);
    if (!wasOriginalAndWasArchived) {
      // The file to be deleted wasn't present at transaction start, so instead
      // of archiving it, we delete it the conventional way, making sure to
      // erase its record in filesToRemoveOnAbort if it was listed there.
      filesToRemoveOnAbort.erase(name);

      RAMDirectory::deleteFile(name, throwError);
    }
  }

  void TransactionalRAMDirectory::renameFile(const char_t* from, const char_t* to) {
    // During the review on 2005.03.18, decided not to implement transactional
    // renameFile for two reasons:
    //  a) It's not needed in the limited scenario for which
    //     TransactionalRAMDirectory was designed (IndexWriter::addDocument and
    //     subcode).
    //  b) Supporting renaming during a transaction would add considerable
    //     bookkeeping overhead, reducing the performance of the overwhelmingly
    //     typical case (commit) in order to support the rare case (abort).
    //
    // This was not a thinly disguised punt due to the complication of
    // implementing renameFile transactionally; rather, several implementations
    // were considered, but it seemed wrongheaded to degrade the performance of
    // the typical case based on the mere potential need to support renameFile
    // at some future point for the benefit of the atypical case.
    if (transOpen) {
      _THROWC("TransactionalRAMDirectory disallows renameFile during a transaction.");
    }
    RAMDirectory::renameFile(from, to);
  }

  OutputStream& TransactionalRAMDirectory::createFile(const char_t* name) {
    if (!transOpen) {
      return RAMDirectory::createFile(name);
    }

    bool wasOriginalAndWasArchived = archiveOrigFileIfNecessary(name);
    try {
      OutputStream& ret = RAMDirectory::createFile(name);
      // Importantly, we store a pointer to the filename memory managed by
      // files, rather than that passed in by the client (name).  We don't make
      // an additional copy of the filename's memory because the transactional
      // metadata container filesToRemoveOnAbort is not at risk of outliving
      // files.
      filesToRemoveOnAbort.insert(files.getKey(name));
      return ret;
    } catch (...) {
      if (wasOriginalAndWasArchived) {
        unarchiveOrigFile(name);
      }
      throw;
    }
  }

  void TransactionalRAMDirectory::close() {
    if (transOpen) {
      transAbort();
    }
    RAMDirectory::close();
  }

}}
