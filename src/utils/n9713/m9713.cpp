#include "n9713/m9713.h"
QVector<double> m9713::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
