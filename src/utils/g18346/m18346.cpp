#include "g18346/m18346.h"
QVector<double> m18346::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
