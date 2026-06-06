#include "s9418/m9418.h"
QVector<double> m9418::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
