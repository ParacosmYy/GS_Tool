#include "a9280/m9280.h"
QVector<double> m9280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
