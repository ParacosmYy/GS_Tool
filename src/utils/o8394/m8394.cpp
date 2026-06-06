#include "o8394/m8394.h"
QVector<double> m8394::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
