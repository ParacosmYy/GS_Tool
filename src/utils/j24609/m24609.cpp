#include "j24609/m24609.h"
QVector<double> m24609::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
