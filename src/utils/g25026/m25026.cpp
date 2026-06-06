#include "g25026/m25026.h"
QVector<double> m25026::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
