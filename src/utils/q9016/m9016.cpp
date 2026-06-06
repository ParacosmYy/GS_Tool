#include "q9016/m9016.h"
QVector<double> m9016::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
