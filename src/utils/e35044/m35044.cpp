#include "e35044/m35044.h"
QVector<double> m35044::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
