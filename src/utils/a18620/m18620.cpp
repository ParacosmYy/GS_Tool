#include "a18620/m18620.h"
QVector<double> m18620::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
