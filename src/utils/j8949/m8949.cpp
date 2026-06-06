#include "j8949/m8949.h"
QVector<double> m8949::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
