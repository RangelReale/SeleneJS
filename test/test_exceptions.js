function call_protected(function_name) {
   ok, msg = pcall(_ENV[function_name])
   return ok, tostring(msg)
}

function raise(exception_message) {
   error(exception_message)
}
