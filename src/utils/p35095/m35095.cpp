#include "p35095/m35095.h"
QVector<double> m35095::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
