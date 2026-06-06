#include "k8470/m8470.h"
QVector<double> m8470::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
