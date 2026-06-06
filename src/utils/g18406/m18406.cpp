#include "g18406/m18406.h"
QVector<double> m18406::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
