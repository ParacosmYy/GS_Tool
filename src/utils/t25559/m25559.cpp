#include "t25559/m25559.h"
QVector<double> m25559::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
