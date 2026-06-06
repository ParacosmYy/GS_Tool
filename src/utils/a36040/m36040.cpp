#include "a36040/m36040.h"
QVector<double> m36040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
