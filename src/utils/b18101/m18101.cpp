#include "b18101/m18101.h"
QVector<double> m18101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
