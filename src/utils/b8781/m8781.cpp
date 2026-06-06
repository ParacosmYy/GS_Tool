#include "b8781/m8781.h"
QVector<double> m8781::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
