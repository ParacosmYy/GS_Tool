#include "j9789/m9789.h"
QVector<double> m9789::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
