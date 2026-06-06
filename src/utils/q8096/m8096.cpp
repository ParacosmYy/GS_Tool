#include "q8096/m8096.h"
QVector<double> m8096::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
