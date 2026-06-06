#include "j8109/m8109.h"
QVector<double> m8109::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
