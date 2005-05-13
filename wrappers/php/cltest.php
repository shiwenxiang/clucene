CLucene PHP wrapper test<BR>
<I>Experimental only</i>
<?php

if ( $_ENV["OS"] == "Windows_NT" )
    dl("php_clucene.dll");
else
    dl("clucene.so");

if ( $HTTP_POST_VARS["location"] != null ){
    $cl = cl_open ($HTTP_POST_VARS["location"],true);
    if ( $cl != null ){
        if ( $HTTP_POST_VARS["action"] == "index" ){
            ?>
            Index results:<BR>
            <?
            if ( !cl_new_document($cl)  ) echo "<BR>New Document error: ".cl_errstr($cl);
            if ( !cl_add_field($cl,"ref", $HTTP_POST_VARS["ndxRef"],true) ) echo "<BR>Reference error: ". cl_errstr($cl);
            if ( !cl_add_field($cl,"cnt", $HTTP_POST_VARS["ndxCnt"])  ) echo "<BR>Content error: ". cl_errstr($cl);
            
            echo "Document to add: ".cl_document_info($cl);
            if ( !cl_insert_document($cl)  )
				echo "<BR>Insert error: ". cl_errstr($cl);
            else
                echo "<BR>Document added!";
            ?>
            
            <HR>
        <?
        
        }elseif ( $HTTP_POST_VARS["action"] == "search" ){
            ?>
            Search results:<BR>

            <? 
                $field = $HTTP_POST_VARS["fld"]."";
                if ( $field == "" )
                    $field = "cnt";
                elseif ( strpos($field,",") > 0 )
                    $field = explode(",",$field);

            if ( cl_search($cl, $HTTP_POST_VARS["s"], $field)){
                echo "Search found ".cl_hitcount($cl)." documents while searching for ".cl_searchinfo($cl)."<BR>";
                
                do{
                    echo "Document: ".cl_getfield($cl,"ref")."<BR>";
                }while( cl_nexthit($cl) );
            }else{
                echo "Search failed - ".cl_errstr($cl)."<BR>";
            }
            ?>
            <HR>
        <?  
        
        
        
        }elseif ( $HTTP_POST_VARS["action"] == "optimize" ){
            ?>
            Optimize results:<BR>
            <? 
                    if ( !cl_optimize($cl) )
                        echo "Optimize failed: ".cl_errstr($cl); 
                    else
                        echo "Optimizing successful...";
            ?>
            <HR>
            <?
        }elseif ( $HTTP_POST_VARS["action"] == "delete" ){
            ?>
            Delete results:<BR>
            
            <?
                $ret = cl_delete($cl,$HTTP_POST_VARS["dv"],$HTTP_POST_VARS["d"]);
               if ( $ret < 0 ){
                    echo "Deleting failed: ".cl_errstr($cl); 
               }else{
                    echo "Deleted $ret documents.";
               }
        }else{
            echo "Unknown search type.";
        }
   }else{
        echo "Opening ${HTTP_POST_VARS["location"]} failed. - " . cl_errstr($cl) . " <BR>";
   }
    cl_close($cl); 

}

?>


<P><B>================================</B>
<form method=post> <!--use a different form so that index text area can use all the text space-->
Directory to work on: <input name=location value="<?=($HTTP_POST_VARS["location"]==null?"index":$HTTP_POST_VARS["location"])?>"><BR>

<P><B>or</B>

<P> <i>Perform a search:</i><input name=s value="<?=($HTTP_POST_VARS["s"]==null?"Search string":$HTTP_POST_VARS["s"])?>"><BR>
    on field (or fields seperated by commas) <input name=fld value="<?=($HTTP_POST_VARS["fld"]==null?"ref,cnt":$HTTP_POST_VARS["fld"])?>"><input type=submit name=action value=search>

<P><B>or</B>

<P> <i>Optimize this index:</i> <input type=submit name=action value=optimize>

<P><B>or</B>

<P> Delete some data from the index:<BR>
Delete from the <select name=d >
    <option value=ref>Reference field</option>
    <option value=cnt>Contents field</option>
</select>
which would return from this query: <input name=dv value="01234">
<input type=submit name=action value=delete>

</form>

<P><B>================================</B>


<form method=post>
Directory to work on: <input name=location value="<?=($HTTP_POST_VARS["location"]==null?"index":$HTTP_POST_VARS["location"])?>"><BR>

<P> <i>Index these values: </i><BR>
<B>Reference: </B><input name=ndxRef value="<?=($HTTP_POST_VARS["ndxRef"]==null?"01234":$HTTP_POST_VARS["ndxRef"])?>">(stored, this could be the file name)<BR>
<B>Data:</B> <textarea name=ndxCnt cols=70 rows=10><?=($HTTP_POST_VARS["ndxCnt"]==null?"Some text to index":$HTTP_POST_VARS["ndxCnt"])?></textarea>(indexed, this could be the contents of a document)<BR>
<input type=submit name=action value=index>

</form>
