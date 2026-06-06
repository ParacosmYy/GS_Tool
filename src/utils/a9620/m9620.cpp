#include "a9620/m9620.h"
QVector<double> m9620::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
