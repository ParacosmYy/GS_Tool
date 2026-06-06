#include "a29700/m29700.h"
QVector<double> m29700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
