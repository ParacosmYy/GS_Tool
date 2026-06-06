#include "p9835/m9835.h"
QVector<double> m9835::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
