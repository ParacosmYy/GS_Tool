#include "m18772/m18772.h"
QVector<double> m18772::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
