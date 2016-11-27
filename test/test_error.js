function err_func1(x, y) {
	err_func2(x + y)
}

function divide_by_zero() {
	return 1 / 0
}

function _overflow(n) {
	return _overflow(n + 1) + 1
}

function do_overflow() {
	_overflow(1)
}
