#include "g18426/m18426.h"
QVector<double> m18426::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
