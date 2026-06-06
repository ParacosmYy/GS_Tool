#include "m18652/m18652.h"
QVector<double> m18652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
