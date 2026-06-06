#include "o32114/m32114.h"
QVector<double> m32114::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
