#include "g8606/m8606.h"
QVector<double> m8606::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
