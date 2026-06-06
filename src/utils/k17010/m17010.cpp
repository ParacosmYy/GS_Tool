#include "k17010/m17010.h"
QVector<double> m17010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
