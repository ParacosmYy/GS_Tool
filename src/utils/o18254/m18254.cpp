#include "o18254/m18254.h"
QVector<double> m18254::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
