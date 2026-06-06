#include "d16523/m16523.h"
QVector<double> m16523::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
