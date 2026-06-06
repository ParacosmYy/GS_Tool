#include "j25609/m25609.h"
QVector<double> m25609::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
