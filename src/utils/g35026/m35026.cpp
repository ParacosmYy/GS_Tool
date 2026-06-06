#include "g35026/m35026.h"
QVector<double> m35026::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
