#include "a32560/m32560.h"
QVector<double> m32560::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
