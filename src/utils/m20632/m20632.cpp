#include "m20632/m20632.h"
QVector<double> m20632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
