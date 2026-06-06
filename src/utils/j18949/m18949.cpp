#include "j18949/m18949.h"
QVector<double> m18949::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
