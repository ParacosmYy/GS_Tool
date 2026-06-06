#include "k25290/m25290.h"
QVector<double> m25290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
