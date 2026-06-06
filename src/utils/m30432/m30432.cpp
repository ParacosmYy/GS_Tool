#include "m30432/m30432.h"
QVector<double> m30432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
