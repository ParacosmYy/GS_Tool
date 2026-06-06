#include "a8880/m8880.h"
QVector<double> m8880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
