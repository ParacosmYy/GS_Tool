#include "n7893/m7893.h"
QVector<double> m7893::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
