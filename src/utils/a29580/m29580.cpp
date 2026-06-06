#include "a29580/m29580.h"
QVector<double> m29580::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
