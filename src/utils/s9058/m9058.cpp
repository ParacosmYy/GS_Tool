#include "s9058/m9058.h"
QVector<double> m9058::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
