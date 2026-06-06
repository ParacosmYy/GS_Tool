#include "l8631/m8631.h"
QVector<double> m8631::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
