ok = true;

function call_protected(function_name) {
   var msg = '';
   ok = true;
   try {
        msg = eval(function_name+'();');
   } catch (e) {
        ok = false;
        msg = e.message;
   }
   return msg;
}

function raise(exception_message) {
   throw new Error(exception_message);
}
