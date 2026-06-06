#include "i9488/m9488.h"
QVector<double> m9488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
