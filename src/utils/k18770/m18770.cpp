#include "k18770/m18770.h"
QVector<double> m18770::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
