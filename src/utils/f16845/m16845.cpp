#include "f16845/m16845.h"
QVector<double> m16845::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
