#include "a29620/m29620.h"
QVector<double> m29620::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
