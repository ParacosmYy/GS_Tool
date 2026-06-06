#include "i30708/m30708.h"
QVector<double> m30708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
