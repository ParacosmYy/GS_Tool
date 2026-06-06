#include "j7949/m7949.h"
QVector<double> m7949::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
