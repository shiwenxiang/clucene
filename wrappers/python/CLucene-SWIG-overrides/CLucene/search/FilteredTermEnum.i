/* Reviewed 2004.08.20: necessary */

/* For the sake of FuzzyQuery.h, we make SWIG aware of FilteredTermEnum, then
** ignore it (Python programmers can accesss it via the generic TermEnum
** interface). */

class FilteredTermEnum: public TermEnum {
  /* Of course the real FilteredTermEnum contains actually declares some members. */
};
