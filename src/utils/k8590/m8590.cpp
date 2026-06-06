#include "k8590/m8590.h"
QVector<double> m8590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
