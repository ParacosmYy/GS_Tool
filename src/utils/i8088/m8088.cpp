#include "i8088/m8088.h"
QVector<double> m8088::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
