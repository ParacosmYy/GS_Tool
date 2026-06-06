#include "a29260/m29260.h"
QVector<double> m29260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
