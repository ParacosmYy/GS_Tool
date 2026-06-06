#include "r21937/m21937.h"
QVector<double> m21937::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
