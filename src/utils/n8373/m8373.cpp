#include "n8373/m8373.h"
QVector<double> m8373::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
