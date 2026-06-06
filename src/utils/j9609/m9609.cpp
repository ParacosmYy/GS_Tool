#include "j9609/m9609.h"
QVector<double> m9609::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
