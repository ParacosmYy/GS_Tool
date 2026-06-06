#include "a32180/m32180.h"
QVector<double> m32180::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
