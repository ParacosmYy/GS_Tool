#include "k8350/m8350.h"
QVector<double> m8350::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
