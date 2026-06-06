#include "f30525/m30525.h"
QVector<double> m30525::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
